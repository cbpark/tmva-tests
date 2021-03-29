use flate2::read::GzDecoder;
use hepurs::data::lhef::event;
use hepurs::kinematics::momentum::{
    invariant_mass, transverse_mass, FourMomentum, HasFourMomentum,
    TransverseMomentum,
};
use hepurs::kinematics::variables::mt2;
use hepurs::kinematics::vector::LorentzTVector;
use std::fs::File;
use std::io::BufReader;
use std::io::Write;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let args: Vec<String> = std::env::args().collect();
    if args.len() != 3 {
        eprintln!("Usage: ttbar-analysis input.lhe.gz output.dat");
        std::process::exit(1);
    }

    let input = read_event_file(&args[1]);
    println!("ttbar-analysis: the input file is {}", args[1]);

    let mut outfile = create_output(&args[2]);
    println!("ttbar-analysis: the output will be stored in {}", args[2]);

    let mut lhef = event::Parser::new(input)?;
    while let Ok(event) = lhef.event() {
        match event {
            None => {
                break;
            }
            Some(e) => {
                if let Some(ps) = Particles::from(e) {
                    writeln!(
                        outfile,
                        "{:12.6} {:12.6} {:12.6}",
                        ps.get_mt2(),
                        ps.get_mbl(),
                        ps.get_mbbll()
                    )?;
                }
            }
        }
    }

    println!("ttbar-analysis: ... done.");
    Ok(())
}

pub fn read_event_file(filename: &str) -> BufReader<GzDecoder<File>> {
    let input_file =
        File::open(filename).expect("Failed to read the LHE file.");
    let event_str = GzDecoder::new(input_file);
    BufReader::new(event_str)
}

pub fn create_output(filename: &str) -> File {
    let out_path = std::path::Path::new(filename);
    File::create(&out_path).expect("Failed to create the output file.")
}

#[derive(Debug)]
struct Particles {
    b1: FourMomentum,
    b2: FourMomentum,
    l1: FourMomentum,
    l2: FourMomentum,
    ptmiss: TransverseMomentum,
}

impl Particles {
    fn from(event: event::Event) -> Option<Particles> {
        let bquarks = event.get(&[5, -5]);
        if bquarks.len() != 2 {
            return None;
        }

        let leptons = event.get(&[11, -11, 13, -13]);
        if leptons.len() != 2 {
            return None;
        }

        let ptmiss = event.pt_miss(&[12, -12, 14, -14]);

        Some(Particles {
            b1: bquarks[0].four_momentum(),
            b2: bquarks[1].four_momentum(),
            l1: leptons[0].four_momentum(),
            l2: leptons[1].four_momentum(),
            ptmiss,
        })
    }

    fn get_mt2(&self) -> f64 {
        let mt2_1 = mt2::mT2symm(
            &(self.b1 + self.l1),
            &(self.b2 + self.l2),
            &self.ptmiss,
            0.0,
        );
        let mt2_2 = mt2::mT2symm(
            &(self.b1 + self.l2),
            &(self.b2 + self.l1),
            &self.ptmiss,
            0.0,
        );
        mt2_1.min(mt2_2)
    }

    fn get_mbl(&self) -> f64 {
        let mbl_1 = [
            invariant_mass(&[self.b1, self.l1]).unwrap_or(0.0),
            invariant_mass(&[self.b2, self.l2]).unwrap_or(0.0),
        ];
        let mbl_max_1 = mbl_1[0].max(mbl_1[1]);

        let mbl_2 = [
            invariant_mass(&[self.b1, self.l2]).unwrap_or(0.0),
            invariant_mass(&[self.b2, self.l1]).unwrap_or(0.0),
        ];
        let mbl_max_2 = mbl_2[0].max(mbl_2[1]);

        mbl_max_1.min(mbl_max_2)
    }

    fn get_mbbll(&self) -> f64 {
        let missing = LorentzTVector::from(
            self.ptmiss.px(),
            self.ptmiss.py(),
            self.ptmiss.pmag(),
        );
        transverse_mass(&(self.b1 + self.b2 + self.l1 + self.l2), &missing)
            .unwrap_or(0.0)
    }
}
